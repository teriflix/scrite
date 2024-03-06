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

import io.scrite.components 1.0

QtObject {

    readonly property Settings scrollArea: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "ScrollArea"
        property real zoomFactor: 0.05
    }

    readonly property Settings structureCanvas: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Structure Tab"

        property bool showGrid: true
        property color gridColor: ScriteAccentColors.c400.background
        property color canvasColor: ScriteAccentColors.c50.background
        property bool showPreview: true
        property bool displayAnnotationProperties: true
        property bool showPullHandleAnimation: true
        property real lineWidthOfConnectors: 1.5
    }

    readonly property Settings timelineView: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Timeline View"

        property string textMode: "HeadingOrTitle"
    }

    readonly property Settings screenplayEditor: Settings {
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
    }

    readonly property Settings pdfExport: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "PdfExport"

        property bool usePdfDriver: true
    }

    readonly property Settings titlePage: Settings {
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

    readonly property Settings richTextEditor: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Rich Text Editor"

        property bool languageNoteShown: false
    }

    readonly property Settings sceneListPanel: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Scene List Panel"

        property string displaySceneLength: "NO" // can be PAGE, TIME
    }

    readonly property Settings markupTools: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Markup Tools"

        property real contentX: 20
        property real contentY: 20
    }

    readonly property Settings scrited: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Scrited"

        property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        property bool experimentalFeatureNoticeDisplayed: false
        property bool codecsNoticeDisplayed: false
        property real playerAreaRatio: 0.5
        property bool videoPlayerVisible: true
    }

    readonly property Settings shortcutsDockWidget: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Shortcuts Dock Widget"

        property real contentX: -1
        property real contentY: -1
        property bool visible: true
    }

    readonly property Settings helpNotification: Settings {
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

    readonly property Settings notebook: Settings {
        fileName: Scrite.app.settingsFilePath
        category: "Notebook"

        property int activeTab: 0 // 0 = Relationships, 1 = Notes
        property int graphLayoutMaxTime: 1000
        property int graphLayoutMaxIterations: 50000
        property bool showAllFormQuestions: true
    }


    readonly property Settings paragraphLanguage: Settings {
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

    readonly property Settings workspace: Settings {
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
        property bool autoOpenLastFile: false
    }

    readonly property Settings application: Settings {
        id: applicationSettings
        fileName: Scrite.app.settingsFilePath
        category: "Application"

        property bool enableAnimations: true
        property bool useSoftwareRenderer: false
        property string theme: "Material"
    }
}
