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

import "qrc:/js/utils.js" as Utils

Item {
    id: root

    visible: false

    function init(_parent) {
        if( !(_parent && Scrite.app.verifyType(_parent, "QQuickItem")) )
            _parent = Scrite.window.contentItem

        parent = _parent
        visible = false
        anchors.fill = parent
    }

    // Global variables
    readonly property real minWindowWidthForShowingNotebookInStructure: 1600
    property bool canShowNotebookInStructure: width > minWindowWidthForShowingNotebookInStructure
    property bool showNotebookInStructure: workspaceSettings.showNotebookInStructure && canShowNotebookInStructure
    property bool firstSwitchToStructureTab: true // This is different from screenplayEditorSettings.firstSwitchToStructureTab
    property ObjectListModel dialogs: ObjectListModel { }

    // Persistent Settings
    readonly property Settings scrollAreaSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "ScrollArea"
        property real zoomFactor: 0.05
    }

    readonly property Settings structureCanvasSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Structure Tab"

        property bool showGrid: true
        property color gridColor: Runtime.colors.accent.c400.background
        property color canvasColor: Runtime.colors.accent.c50.background
        property bool showPreview: true
        property bool displayAnnotationProperties: true
        property bool showPullHandleAnimation: true
        property real lineWidthOfConnectors: 1.5

        function restoreDefaultGridColor() {
            gridColor = Runtime.colors.accent.c400.background
        }

        function restoreDefaultCanvasColor() {
            canvasColor = Runtime.colors.accent.c50.background
        }
    }

    readonly property Settings timelineViewSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Timeline View"

        property string textMode: "HeadingOrTitle"
    }

    readonly property Settings screenplayEditorSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Screenplay Editor"

        property bool screenplayEditorAddButtonsAnimationShown: false
        property bool refreshButtonInStatsReportAnimationDone: false
        property bool firstSwitchToStructureTab: true
        property bool displayRuler: true
        property bool displaySceneCharacters: true
        property bool displaySceneSynopsis: true
        property bool displaySceneComments: false
        property bool displayEmptyTitleCard: true
        property bool displayAddSceneBreakButtons: true
        property bool displayIndexCardFields: true
        property int mainEditorZoomValue: -1
        property int embeddedEditorZoomValue: -1
        property bool autoAdjustEditorWidthInScreenplayEditor: true
        property var zoomLevelModifiers: { "tab0": 0, "tab1": 0, "tab2": 0, "tab3": 0 }
        property bool includeTitlePageInPreview: true
        property bool singleClickAutoComplete: true
        property bool enableSpellCheck: true // Since this is now fixed: https://github.com/teriflix/scrite/issues/138
        property bool enableAutoCapitalizeSentences: true
        property bool enableAutoPolishParagraphs: true // for automatically adding/removing CONT'D where appropriate
        property int lastLanguageRefreshNoticeBoxTimestamp: 0
        property int lastSpellCheckRefreshNoticeBoxTimestamp: 0
        property bool showLanguageRefreshNoticeBox: true
        property bool showSpellCheckRefreshNoticeBox: true
        property bool showLoglineEditor: false
        property bool allowTaggingOfScenes: false
        property real spaceBetweenScenes: 0
        property int commentsPanelTabIndex: 1
        property bool markupToolsDockVisible: true
        property bool pausePageAndTimeComputation: false
        property bool highlightCurrentLine: true
        property bool applyUserDefinedLanguageFonts: true
        property bool optimiseScrolling: false

        property bool copyAsFountain: true
        property bool copyFountainUsingStrictSyntax: true
        property bool copyFountainWithEmphasis: true

        property bool pasteAsFountain: true
        property bool pasteByMergingAdjacentElements: true
        property bool pasteAfterResolvingEmphasis: true
    }

    readonly property Settings pdfExportSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "PdfExport"

        property bool usePdfDriver: true
    }

    readonly property Settings titlePageSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "TitlePage"

        property string author
        property string contact
        property string address
        property string email
        property string phone
        property string website
        property bool includeTimestamp: false
    }

    readonly property Settings richTextEditorSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Rich Text Editor"

        property bool languageNoteShown: false
    }

    readonly property Settings sceneListPanelSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Scene List Panel"

        property string displaySceneLength: "NO" // can be PAGE, TIME
        property string sceneTextMode: "HEADING" // can be SUMMARY also
        property bool showTooltip: false
    }

    readonly property Settings markupToolsSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Markup Tools"

        property real contentX: 20
        property real contentY: 20
    }

    readonly property Settings scritedSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Scrited"

        property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        property bool experimentalFeatureNoticeDisplayed: false
        property bool codecsNoticeDisplayed: false
        property real playerAreaRatio: 0.5
        property bool videoPlayerVisible: true
    }

    readonly property Settings shortcutsDockWidgetSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Shortcuts Dock Widget"

        property real contentX: -1
        property real contentY: -1
        property bool visible: true
    }

    readonly property Settings helpNotificationSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Help"

        property string dayZero
        function daysSinceZero() {
            const today = new Date()
            const dzero = dayZero === "" ? today : new Date(dayZero + "Z")
            const days = Math.floor((today.getTime() - dzero.getTime()) / (24*60*60*1000))
            return days
        }

        property string tipsShown: ""
        function isTipShown(val) {
            const ts = tipsShown.split(",")
            return ts.indexOf(val) >= 0
        }
        function markTipAsShown(val) {
            var ts = tipsShown.length > 0 ? tipsShown.split(",") : []
            if(ts.indexOf(val) < 0)
                ts.push(val)
            tipsShown = ts.join(",")
        }
    }

    readonly property Settings notebookSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Notebook"

        property int activeTab: 0 // 0 = Relationships, 1 = Notes
        property int graphLayoutMaxTime: 1000
        property int graphLayoutMaxIterations: 50000
        property bool showAllFormQuestions: true
        property bool richTextNotesEnabled: true
    }

    readonly property Settings paragraphLanguageSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Paragraph Language"

        property string shotLanguage: "Default"
        property string actionLanguage: "Default"
        property string defaultLanguage: "English"
        property string dialogueLanguage: "Default"
        property string characterLanguage: "Default"
        property string transitionLanguage: "Default"
        property string parentheticalLanguage: "Default"
    }

    readonly property Settings workspaceSettings: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Workspace"

        property real workspaceHeight
        property real screenplayEditorWidth: -1
        property bool scriptalayIntroduced: false
        property bool showNotebookInStructure: true
        property bool syncCurrentSceneOnNotebook: true
        property bool animateStructureIcon: true
        property bool animateNotebookIcon: true
        property real flickScrollSpeedFactor: 1.0
        property bool showScritedTab: false
        property bool mouseWheelZoomsInCharacterGraph: Scrite.app.isWindowsPlatform || Scrite.app.isLinuxPlatform
        property bool mouseWheelZoomsInStructureCanvas: Scrite.app.isWindowsPlatform || Scrite.app.isLinuxPlatform
        property string lastOpenFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenPhotosFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        property string lastOpenImportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenExportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenReportsFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        property var customColors: []
        property var defaultSceneColor: Scrite.app.standardColors[0]
        property bool autoOpenLastFile: false
    }

    readonly property Settings applicationSettings: Settings {
        id: applicationSettings
        fileName: Scrite.app.settingsFilePath
        category: "Application"

        property bool enableAnimations: true
        property bool useSoftwareRenderer: false
        property string theme: "Material"
        property int primaryColor: colors.defaultPrimaryColor
        property int accentColor: colors.defaultAccentColor
        property int joinDiscordPromptCounter: 0

        Component.onCompleted: {
            Qt.callLater( () => {
                             Runtime.currentTheme = theme
                             Runtime.currentUseSoftwareRenderer = useSoftwareRenderer
                         })
        }
    }

    property string currentTheme
    property bool currentUseSoftwareRenderer

    // Global undo-redo object
    readonly property UndoStack undoStack: UndoStack {
        objectName: "MainUndoStack"

        property bool sceneListPanelActive: false
        property bool screenplayEditorActive: false
        property bool timelineEditorActive: false
        property bool structureEditorActive: false
        property bool sceneEditorActive: false
        property bool notebookActive: false

        active: sceneListPanelActive || screenplayEditorActive || timelineEditorActive || structureEditorActive || sceneEditorActive || notebookActive
    }

    // App-wide font-metrics
    readonly property FontMetrics minimumFontMetrics: FontMetrics {
        font.pointSize: Math.min(Scrite.app.idealFontPointSize-2, 12)
    }

    readonly property FontMetrics idealFontMetrics: FontMetrics {
        font.pointSize: Scrite.app.idealFontPointSize
    }

    readonly property FontMetrics sceneEditorFontMetrics: FontMetrics {
        property SceneElementFormat format: Scrite.document.formatting.elementFormat(SceneElement.Action)
        property int lettersPerLine: 70
        property int marginLetters: 5
        property real paragraphWidth: Math.ceil(lettersPerLine*averageCharacterWidth)
        property real paragraphMargin: Math.ceil(marginLetters*averageCharacterWidth)
        property real pageWidth: Math.ceil(paragraphWidth + 2*paragraphMargin)
        font: format ? format.font2 : Scrite.document.formatting.defaultFont2
    }

    // Color Palettes
    component Colors : QtObject {
        property int key: Material.Indigo
        property color windowColor: c300.background
        property color borderColor: c400.background
        property color separatorColor: c400.background
        property var highlight: c400
        property var button: c200

        property QtObject regular: QtObject {
            property color background: Material.color(key)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c10: QtObject {
            property color background: Qt.rgba(1,1,1,0)
            property color text: "black"
        }

        property QtObject c50: QtObject {
            property color background: Material.color(key, Material.Shade50)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c100: QtObject {
            property color background: Material.color(key, Material.Shade100)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c200: QtObject {
            property color background: Material.color(key, Material.Shade200)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c300: QtObject {
            property color background: Material.color(key, Material.Shade300)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c400: QtObject {
            property color background: Material.color(key, Material.Shade400)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c500: QtObject {
            property color background: Material.color(key, Material.Shade500)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c600: QtObject {
            property color background: Material.color(key, Material.Shade600)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c700: QtObject {
            property color background: Material.color(key, Material.Shade700)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c800: QtObject {
            property color background: Material.color(key, Material.Shade800)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject c900: QtObject {
            property color background: Material.color(key, Material.Shade900)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject a100: QtObject {
            property color background: Material.color(key, Material.ShadeA100)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject a200: QtObject {
            property color background: Material.color(key, Material.ShadeA200)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject a400: QtObject {
            property color background: Material.color(key, Material.ShadeA400)
            property color text: Scrite.app.textColorFor(background)
        }

        property QtObject a700: QtObject {
            property color background: Material.color(key, Material.ShadeA700)
            property color text: Scrite.app.textColorFor(background)
        }
    }

    readonly property QtObject colors: Item {
        readonly property int theme: Material.Light
        readonly property int defaultPrimaryColor: Material.Grey
        readonly property int defaultAccentColor: Material.DeepPurple

        readonly property Colors primary: Colors {
            key: Material.Grey // applicationSettings.primaryColor
        }

        readonly property Colors accent: Colors {
            key: applicationSettings.accentColor
        }

        readonly property color transparent: "transparent"
        readonly property var   forDocument: ["#e60000", "#ff9900", "#ffff00", "#008a00", "#0066cc", "#9933ff", "#ffffff", "#facccc", "#ffebcc", "#ffffcc", "#cce8cc", "#cce0f5", "#ebd6ff", "#bbbbbb", "#f06666", "#ffc266", "#ffff66", "#66b966", "#66a3e0", "#c285ff", "#888888", "#a10000", "#b26b00", "#b2b200", "#006100", "#0047b2", "#6b24b2", "#444444", "#5c0000", "#663d00", "#666600", "#003700", "#002966", "#3d1466"]
        readonly property var   forScene: Scrite.app.standardColors(Scrite.app.versionNumber)
    }

    // All the app-features
    readonly property QtObject appFeatures: QtObject {
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
    }

    // These properties are only accessible at runtime, which means they are not
    // stored in persistent settings file.
    readonly property int e_ScreenplayTab: 0
    readonly property int e_StructureTab: 1
    readonly property int e_NotebookTab: 2
    readonly property int e_ScritedTab: 3
    property int mainWindowTab: e_ScreenplayTab
    signal activateMainWindowTab(int tabType)

    property bool loadMainUiContent: true

    readonly property var characterReports: {
        let reports = Scrite.document.supportedReports
        let ret = []
        reports.forEach( function(item) {
            if(item.name.indexOf('Character') >= 0)
                ret.push(item)
        })
        return ret
    }

    // This model provides access to recently accessed files. It is updated from
    // different parts of the UI where opening / saving of files is triggered.
    // Contents of this model is listed in the HomeScreen.
    readonly property ScriteFileListModel recentFiles: ScriteFileListModel {
        id: _recentFiles
        source: ScriteFileListModel.RecentFiles

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
        }

        function captureChangeInFiles() {
            _recentFilesSettings.files = files
        }
    }

    // This model is how the screenplay of the current ScriteDocument is accessed.
    readonly property ScreenplayAdapter screenplayAdapter: ScreenplayAdapter {
        property string sessionId
        source: {
            if(Scrite.document.sessionId !== sessionId)
            return null

            if(mainWindowTab === e_ScreenplayTab)
            return Scrite.document.screenplay

            if(Scrite.document.screenplay.currentElementIndex < 0) {
                var index = Scrite.document.structure.currentElementIndex
                var element = Scrite.document.structure.elementAt(index)
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

    // This property holds reference to an instance of ScreenplayEditor
    property Item screenplayEditor

    // This property holds reference to the global screenplay editor toolbar
    property Item screenplayEditorToolbar

    // This model provides access to the paginated-text-document constructed from the screenplay
    // of the current Scrite file.
    readonly property ScreenplayTextDocument screenplayTextDocument: ScreenplayTextDocument {
        // Setting this is as good as setting the other.
        // when paused = true, page and time computation is halted.
        property bool paused: Runtime.screenplayEditorSettings.pausePageAndTimeComputation
        onPausedChanged: Qt.callLater( function() {
            Runtime.screenplayEditorSettings.pausePageAndTimeComputation = screenplayTextDocument.paused
        })

        screenplay: Scrite.document.loading || paused ? null : Runtime.screenplayAdapter.screenplay
        formatting: Scrite.document.loading || paused ? null : Scrite.document.printFormat
        syncEnabled: true
        sceneNumbers: false
        titlePage: false
        sceneIcons: false
        listSceneCharacters: false
        includeSceneSynopsis: false
        printEachSceneOnANewPage: false
        secondsPerPage: Scrite.document.printFormat.secondsPerPage

        // FIXME: Do we really need this?
        Component.onCompleted: Scrite.app.registerObject(screenplayTextDocument, "screenplayTextDocument")
    }

    readonly property ScreenplayTracks screenplayTracks : ScreenplayTracks {
        screenplay: Scrite.document.screenplay
        Component.onCompleted: Scrite.app.registerObject(screenplayTracks, "screenplayTracks")
    }

    // Announcement IDs
    readonly property QtObject announcementIds: QtObject {
        readonly property string englishFontFamilyChanged: "763E8FAD-8681-4F64-B574-F9BB7CF8A7F1"
        readonly property string reloadMainUiRequest: "9a7f0f35-346f-461d-bb85-f5c6dc08a01d"
        readonly property string loginRequest: "97369507-721E-4A7F-886C-4CE09A5BCCFB"
        readonly property string focusRequest: "2E3BBE4F-05FE-49EE-9C0E-3332825B72D8"
        readonly property string closeHomeScreenRequest: "4F8F6B5B-5BEB-4D01-97BA-B0018241BD38"
        readonly property string characterNotesRequest: "7D6E5070-79A0-4FEE-8B5D-C0E0E31F1AD8"
        readonly property string sceneNotesRequest: "41EE5E06-FF97-4DB6-B32D-F938418C9529"
        readonly property string notebookNodeRequest: "1dc67418-2584-4598-a68a-de5205bbc028"
        readonly property string sceneTextEditorReceivedFocus: "598e1699-465b-40d5-8cf4-e9753e2c16e7"
        readonly property string closeDialogBoxRequest: "a6456a87-fc8c-405b-bdd7-7625f86272ba"
    }

    readonly property QtObject announcementData: QtObject {
        readonly property QtObject focusOptions: QtObject {
            readonly property string sceneSynopsis: "Scene Synopsis"
            readonly property string addMuteCharacter: "Add Mute Character"
            readonly property string sceneHeading: "Scene Heading"
            readonly property string sceneNumber: "Scene Number"
            readonly property string scene: "Scene"
        }
    }

    // Global file-manager
    readonly property FileManager fileNamager: FileManager {

    }

    // Private objects
    Settings {
        id: _recentFilesSettings
        fileName: Scrite.app.settingsFilePath
        category: "RecentFiles"

        property var files: []
    }
}
