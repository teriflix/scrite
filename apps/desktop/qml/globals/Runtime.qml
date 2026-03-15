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

pragma Singleton

import QtQuick
import QtCore
import QtQuick.Controls.Material

import io.scrite.components

import "./runtime"

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

    readonly property url userGuidesUrl: "https://www.scrite.io/docs/userguide/"

    property int mainWindowTab: Runtime.MainWindowTab.ScreenplayTab
    property int placeholderInterval: bounded(50, Runtime.screenplayEditorSettings.placeholderInterval, 1000)
    property int visibleTooltipCount: 0

    property var helpTips: undefined

    property bool allowAppUsage: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription
    property bool canShowNotebookInStructure: width > minWindowWidthForShowingNotebookInStructure
    property bool currentUseSoftwareRenderer
    property bool loadMainUiContent: true
    property bool showNotebookInStructure: workspaceSettings.showNotebookInStructure && canShowNotebookInStructure
    property bool allowDiacriticEditing: Platform.isMacOSDesktop ? (screenplayEditorSettings.allowDiacriticEditing && !language.activeTransliterationIsInApp) : false

    property string currentTheme

    // This property holds reference to an instance of ScreenplayEditor
    property Item screenplayEditor

    // This property holds reference to an instance of StructureView
    property Item structureView

    // This property holds reference to the global screenplay editor toolbar
    property Item screenplayEditorToolbar

    property ObjectListModel dialogs: ObjectListModel { }

    readonly property Language_RT language: Language_RT { }

    // Persistent Settings
    readonly property UserAccountDialogSettings_RT userAccountDialogSettings: UserAccountDialogSettings_RT { }
    readonly property ScrollAreaSettings_RT scrollAreaSettings: ScrollAreaSettings_RT { }
    readonly property StructureCanvasSettings_RT structureCanvasSettings: StructureCanvasSettings_RT { }
    readonly property TimelineViewSettings_RT timelineViewSettings: TimelineViewSettings_RT { }
    readonly property ScreenplayEditorSettings_RT screenplayEditorSettings: ScreenplayEditorSettings_RT { }
    readonly property ScreenplayTracksSettings_RT screenplayTracksSettings: ScreenplayTracksSettings_RT { }
    readonly property PdfExportSettings_RT pdfExportSettings: PdfExportSettings_RT { }
    readonly property TitlePageSettings_RT titlePageSettings: TitlePageSettings_RT { }
    readonly property RichTextEditorSettings_RT richTextEditorSettings: RichTextEditorSettings_RT { }
    readonly property SceneListPanelSettings_RT sceneListPanelSettings: SceneListPanelSettings_RT { }
    readonly property MarkupToolsSettings_RT markupToolsSettings: MarkupToolsSettings_RT { }
    readonly property ScritedSettings_RT scritedSettings: ScritedSettings_RT { }
    readonly property HelpNotificationSettings_RT helpNotificationSettings: HelpNotificationSettings_RT { }
    readonly property NotebookSettings_RT notebookSettings: NotebookSettings_RT { }
    readonly property WorkspaceSettings_RT workspaceSettings: WorkspaceSettings_RT { }

    readonly property ApplicationSettings_RT applicationSettings: ApplicationSettings_RT {
        id: _applicationSettings
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

    readonly property Colors_RT colors: Colors_RT { 
        applicationSettings: _applicationSettings 
    }

    readonly property AppFeatures_RT appFeatures: AppFeatures_RT { }
    readonly property RecentFiles_RT recentFiles: RecentFiles_RT {
        applicationSettings: _applicationSettings
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

    readonly property ScreenplayPaginator_RT paginator : ScreenplayPaginator_RT {
        currentElement: screenplay !== null ? root.screenplayAdapter.currentElement : null
        screenplay: root.screenplayAdapter.screenplay
    }

    // Announcement IDs
    readonly property AnnouncementIds_RT announcementIds: AnnouncementIds_RT { }

    // Global file-manager
    readonly property FileManager fileNamager: FileManager { }

    // Global user guide search index
    readonly property UserGuideSearchIndex userGuideSearchIndex: UserGuideSearchIndex { }

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
        const defaultImports = ["QtQuick",
                                "QtQuick.Controls",
                                "QtQuick.Controls.Material",
                                "io.scrite.components",
                                "\"../globals\"",
                                "\"../helpers\"",
                                "\"../controls\""]
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
        let itemInstance = instance as Item
        let instanceSize = Qt.size(0, 0)
        if(itemInstance) {
            instanceSize = Qt.size(itemInstance.width, itemInstance.height)
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
        let timer = Qt.createQmlObject("import QtQml 2.15; Timer { }", contextObject ? contextObject : root);
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

        let annotObject = Qt.createQmlObject("import io.scrite.components 1.0; Annotation { objectName: \"ica\" }", parent)
        let annot = annotObject as Annotation
        annot.type = type
        annot.geometry = geometry
        if(config) {
            for(let member in config)
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

            Scrite.document.displayFormat.activeLanguageCode = Runtime.language.activeCode
        }

        function onJustLoaded() {
            Runtime.screenplayAdapter.sessionId = Scrite.document.sessionId
        }
    }
}
